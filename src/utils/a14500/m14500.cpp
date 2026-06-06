#include "a14500/m14500.h"
QVector<double> m14500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
