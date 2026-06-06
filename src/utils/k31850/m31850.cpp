#include "k31850/m31850.h"
QVector<double> m31850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
