#ifndef TRACKINGDEVIATIONCALCULATOR_H
#define TRACKINGDEVIATIONCALCULATOR_H

#include <QElapsedTimer>
#include <opencv2/core.hpp>

#include "can/message/canmessagegeneric.h"

class TrackingDeviationCalculator
{
public:
    TrackingDeviationCalculator() = default;

    TrackingDeviation calculate(const cv::Mat &frame,
                                const cv::Rect &trackingROI,
                                const CameraFov &cameraFov) const;

    void processAndSend(const cv::Mat &frame,
                        const cv::Rect &trackingROI,
                        const CameraFov &cameraFov);

    void sendStop();

    void setControlParams(float kpYaw,
                          float kpPitch,
                          float maxCmd,
                          float deadbandDeg);

    void setSendPeriodMs(int periodMs);
    void setMaxDeviationDeg(float maxYawDeg, float maxPitchDeg);

private:
    //void sendPlatformFloatCommand(uint8_t paramId, float value);
    void sendTrackingErrorDeg(uint8_t paramId, float value);
    float limitDeviationDeg(float valueDeg, float maxAbsDeg) const;

private:
    float m_kpYaw = 0.15f;
    float m_kpPitch = 0.15f;
    float m_maxCmd = 1.0f;
    float m_deadbandDeg = 0.02f;

    QElapsedTimer m_sendTimer;
    int m_sendPeriodMs = 50; // 20 Гц

    bool m_stopSent = false;
    float m_maxYawDeviationDeg = 0.5f;
    float m_maxPitchDeviationDeg = 0.5f;
};

#endif // TRACKINGDEVIATIONCALCULATOR_H
