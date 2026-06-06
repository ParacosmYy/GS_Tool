#include "k31970/m31970.h"
QVector<double> m31970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
