#include "m30412/m30412.h"
QVector<double> m30412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
