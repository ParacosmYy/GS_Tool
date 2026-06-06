#include "k20530/m20530.h"
QVector<double> m20530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
