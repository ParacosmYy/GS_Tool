#include "i8268/m8268.h"
QVector<double> m8268::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
