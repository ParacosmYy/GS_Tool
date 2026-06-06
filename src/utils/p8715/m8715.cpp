#include "p8715/m8715.h"
QVector<double> m8715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
