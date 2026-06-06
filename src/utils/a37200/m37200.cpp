#include "a37200/m37200.h"
QVector<double> m37200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
