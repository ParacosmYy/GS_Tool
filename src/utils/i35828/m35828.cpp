#include "i35828/m35828.h"
QVector<double> m35828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
