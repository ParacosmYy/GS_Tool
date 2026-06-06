#include "a28500/m28500.h"
QVector<double> m28500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
