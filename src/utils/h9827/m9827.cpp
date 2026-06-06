#include "h9827/m9827.h"
QVector<double> m9827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
