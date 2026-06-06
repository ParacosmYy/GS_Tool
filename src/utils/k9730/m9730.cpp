#include "k9730/m9730.h"
QVector<double> m9730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
