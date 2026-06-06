#include "n9373/m9373.h"
QVector<double> m9373::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
