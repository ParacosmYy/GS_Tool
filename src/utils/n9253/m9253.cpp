#include "n9253/m9253.h"
QVector<double> m9253::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
