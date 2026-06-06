#include "l28831/m28831.h"
QVector<double> m28831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
