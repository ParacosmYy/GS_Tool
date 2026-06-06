#include "k28810/m28810.h"
QVector<double> m28810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
