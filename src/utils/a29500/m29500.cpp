#include "a29500/m29500.h"
QVector<double> m29500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
