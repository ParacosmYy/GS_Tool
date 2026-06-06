#include "l19011/m19011.h"
QVector<double> m19011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
