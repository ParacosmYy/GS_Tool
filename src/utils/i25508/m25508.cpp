#include "i25508/m25508.h"
QVector<double> m25508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
