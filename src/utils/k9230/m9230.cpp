#include "k9230/m9230.h"
QVector<double> m9230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
