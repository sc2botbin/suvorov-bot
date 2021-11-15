#pragma once

#include <vector>


class Node
{

public:
    enum class Status
    {
        INVALID,
        SUCCESS,
        FAILURE,
        RUNNING
    };

    virtual ~Node();

    virtual Status Run() = 0;
    virtual void Start() {}
    virtual void Finish(Status status_) { status_; }

    Status Update();

    bool IsSuccess() const;
    bool IsFailure() const;
    bool IsRunning() const;
    bool IsFinished() const;
    void Reset();

protected:
    Status m_status = Status::INVALID;

};
