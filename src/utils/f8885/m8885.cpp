#include "f8885/m8885.h"
QVector<double> m8885::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
