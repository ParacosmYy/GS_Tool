#include "i8528/m8528.h"
QVector<double> m8528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
