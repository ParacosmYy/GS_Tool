#include "b15581/m15581.h"
QVector<double> m15581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
