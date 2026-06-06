#include "m24572/m24572.h"
QVector<double> m24572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
