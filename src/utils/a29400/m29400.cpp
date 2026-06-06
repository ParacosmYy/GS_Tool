#include "a29400/m29400.h"
QVector<double> m29400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
