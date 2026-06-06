#include "f10765/m10765.h"
QVector<double> m10765::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
