#include "k20970/m20970.h"
QVector<double> m20970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
