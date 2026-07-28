#include "trackingdeviationcalculator.h"

// УВАГА: шлях може відрізнятися у твоєму проєкті.
// Якщо буде помилка include, покажеш, і ми підставимо правильний шлях.
#include "can/transport/SendDataFrame.h"

#include <QDebug>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

TrackingDeviation TrackingDeviationCalculator::calculate(
    const cv::Mat &frame,
    const cv::Rect &trackingROI,
    const CameraFov &cameraFov) const
{
    TrackingDeviation dev;

    if (frame.empty())
        return dev;

    if (trackingROI.empty())
        return dev;

    if (cameraFov.hDeg <= 0.0f || cameraFov.vDeg <= 0.0f)
        return dev;

    const float frameW = static_cast<float>(frame.cols);
    const float frameH = static_cast<float>(frame.rows);

    const float frameCx = frameW * 0.5f;
    const float frameCy = frameH * 0.5f;

    const float targetCx = trackingROI.x + trackingROI.width * 0.5f;
    const float targetCy = trackingROI.y + trackingROI.height * 0.5f;

    // dxPx > 0 — ціль правіше центра кадру
    // dyPx > 0 — ціль вище центра кадру
    dev.dxPx = targetCx - frameCx;
    dev.dyPx = frameCy - targetCy;

    // Пікселі -> градуси
    dev.yawDeg = dev.dxPx * (cameraFov.hDeg / frameW);
    dev.pitchDeg = dev.dyPx * (cameraFov.vDeg / frameH);

    // Простий P-регулятор
    if (std::fabs(dev.yawDeg) > m_deadbandDeg)
        dev.yawCmd = m_kpYaw * dev.yawDeg;
    else
        dev.yawCmd = 0.0f;

    if (std::fabs(dev.pitchDeg) > m_deadbandDeg)
        dev.pitchCmd = m_kpPitch * dev.pitchDeg;
    else
        dev.pitchCmd = 0.0f;

    dev.yawCmd = std::clamp(dev.yawCmd, -m_maxCmd, m_maxCmd);
    dev.pitchCmd = std::clamp(dev.pitchCmd, -m_maxCmd, m_maxCmd);

    return dev;
}


//обмеження величини відхилення перед відправкою в CAN.
//Так платформа не отримає одразу велике відхилення і не почне різко компенсувати.
float TrackingDeviationCalculator::limitDeviationDeg(float valueDeg, float maxAbsDeg) const
{
    if (maxAbsDeg <= 0.0f)
        return valueDeg;

    if (valueDeg > maxAbsDeg)
        return maxAbsDeg;

    if (valueDeg < -maxAbsDeg)
        return -maxAbsDeg;

    return valueDeg;
}

//Функція налаштування
void TrackingDeviationCalculator::setMaxDeviationDeg(float maxYawDeg, float maxPitchDeg)
{
    if (maxYawDeg > 0.0f)
        m_maxYawDeviationDeg = maxYawDeg;

    if (maxPitchDeg > 0.0f)
        m_maxPitchDeviationDeg = maxPitchDeg;

    qDebug() << "[TRACK DEV LIMIT]"
             << "maxYawDeg =" << m_maxYawDeviationDeg
             << "maxPitchDeg =" << m_maxPitchDeviationDeg;
}

void TrackingDeviationCalculator::processAndSend(
    const cv::Mat &frame,
    const cv::Rect &trackingROI,
    const CameraFov &cameraFov)
{
    TrackingDeviation dev = calculate(frame, trackingROI, cameraFov);

    if (!m_sendTimer.isValid()) {
        m_sendTimer.start();
    } else {
        if (m_sendTimer.elapsed() < m_sendPeriodMs)
            return;

        m_sendTimer.restart();
    }

    const float yawTxDeg =
        limitDeviationDeg(dev.yawDeg, m_maxYawDeviationDeg);

    const float pitchTxDeg =
        limitDeviationDeg(dev.pitchDeg, m_maxPitchDeviationDeg);

    sendTrackingErrorDeg(0x11, yawTxDeg);    // горизонталь, градуси, обмежено
    sendTrackingErrorDeg(0x21, pitchTxDeg);  // вертикаль, градуси, обмежено

    m_stopSent = false;

    qDebug() << "[TRACK DEV DEG TX]"
             << "dxPx =" << dev.dxPx
             << "dyPx =" << dev.dyPx
             << "yawDegRaw =" << dev.yawDeg
             << "pitchDegRaw =" << dev.pitchDeg
             << "yawDegTx =" << yawTxDeg
             << "pitchDegTx =" << pitchTxDeg
             << "FOV =" << cameraFov.hDeg << cameraFov.vDeg;
}

void TrackingDeviationCalculator::sendTrackingErrorDeg(uint8_t paramId, float valueDeg)
{
    uint8_t bytes[4] = {0};

    std::memcpy(bytes, &valueDeg, sizeof(float));

    std::vector<uint8_t> payload = {
        0x00,       // Part
        paramId,    // 0x11 horizontal error deg, 0x21 vertical error deg
        0x02,       // Type = Float
        0x00,       // Action = SET

        bytes[0],
        bytes[1],
        bytes[2],
        bytes[3]
    };

    SendDataFrame::getInstance().Send(0x218, 0x08, payload);

    qDebug() << "[CAN TX TRACK DEG]"
             << "CAN_ID = 0x218"
             << "paramId =" << QString("0x%1").arg(paramId, 2, 16, QChar('0'))
             << "valueDeg =" << valueDeg;
}


void TrackingDeviationCalculator::sendStop()
{
    if (m_stopSent)
        return;

    sendTrackingErrorDeg(0x11, 0.0f);
    sendTrackingErrorDeg(0x21, 0.0f);

    m_stopSent = true;
    m_sendTimer.invalidate();

    qDebug() << "[CAN TX TRACK DEG] STOP";
}

void TrackingDeviationCalculator::setControlParams(float kpYaw,
                                                   float kpPitch,
                                                   float maxCmd,
                                                   float deadbandDeg)
{
    m_kpYaw = kpYaw;
    m_kpPitch = kpPitch;
    m_maxCmd = maxCmd;
    m_deadbandDeg = deadbandDeg;
}

void TrackingDeviationCalculator::setSendPeriodMs(int periodMs)
{
    if (periodMs < 10)
        periodMs = 10;

    m_sendPeriodMs = periodMs;
}
