#include "f25645/m25645.h"
QVector<double> m25645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
