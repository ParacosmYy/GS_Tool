#include "k18970/m18970.h"
QVector<double> m18970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
