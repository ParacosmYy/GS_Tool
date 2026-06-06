#include "l28551/m28551.h"
QVector<double> m28551::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
