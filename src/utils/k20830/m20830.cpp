#include "k20830/m20830.h"
QVector<double> m20830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
