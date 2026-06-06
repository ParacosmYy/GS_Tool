#include "h25567/m25567.h"
QVector<double> m25567::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
