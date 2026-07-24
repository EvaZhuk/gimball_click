// canparserworker.cpp
#include "canparserworker.h"
#include <QDebug>
#include "can/message/canmessagegeneric.h"
//#include "lpsparameters.h"
#include "qmutex.h"

CANParserWorker::CANParserWorker(QObject *parent)
    : QObject(parent)
{}

void CANParserWorker::enqueueMessage(const std::vector<uint8_t> &message)
{
    QMutexLocker locker(&mutex);
    queue.enqueue(message);
}

void CANParserWorker::process()
{
    while (running) {
        mutex.lock();
        if (queue.isEmpty()) {
            mutex.unlock();
            QThread::msleep(10); // Щоб не грузити CPU
            continue;
        }

        std::vector<uint8_t> message = queue.dequeue();
        mutex.unlock();

        // Тут розбір повідомлення:
        try {
            // 🎯 Actual message processing:
            CanMessageGeneric canMessage(message);
            switch (canMessage.Message.TYPE) {
            case ParamType::NoneType:
                if(canMessage.StopTrack()){
                    emit stopTrackingReceived();
                    qDebug() << "[CAN RX] STOP_TRACK";
                }
                break;
            case ParamType::UShort:
            {
                // TrackingParams trackingParams;
                // if (canMessage.ParseTrackingParams(trackingParams)) {
                //     qDebug() << "[CAN RX] tracking params updated:"
                //              << "roiSize =" << trackingParams.roiSize;

                //     emit trackingParamsReceived(trackingParams.roiSize);
                //     return;
                // }
            }
                break;
            case ParamType::UShort2:
            {
                ClickPoint pt;
                if (canMessage.ParseCapturePoint(pt)) {

                    const float nx = static_cast<float>(pt.x) / 65535.0f;
                    const float ny = static_cast<float>(pt.y) / 65535.0f;

                    //emit capturePointReceived(nx, ny);
                    emit capturePointNormalizedReceived(nx, ny);

                    qDebug() << "[CAN RX] CAPTURE_POINT_NORM:"
                             << "rawX =" << pt.x
                             << "rawY =" << pt.y
                             << "nx =" << nx
                             << "ny =" << ny;
                }
                break;
            }
            case ParamType::Float:
            {
                CameraFov cameraFov;
                if (canMessage.ParseFOV(cameraFov)) {
                    qDebug() << "[CAN RX] FOV updated:"
                             << "H =" << cameraFov.hDeg
                             << "V =" << cameraFov.vDeg;

                    emit cameraFovReceived(cameraFov.hDeg, cameraFov.vDeg);
                    return;
                }
                break;
            }
            default:
                break;
            }

            //qDebug() << "Parsing CAN message in thread. Size:" << message.size();
            emit messageParsed();
        } catch (const std::exception &e) {
            emit parseError(QString("Parser error: %1").arg(e.what()));
        }
    }
}
