#include "k15970/m15970.h"
QVector<double> m15970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
