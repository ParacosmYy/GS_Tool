#include "a19700/m19700.h"
QVector<double> m19700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
