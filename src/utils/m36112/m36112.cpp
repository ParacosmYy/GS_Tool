#include "m36112/m36112.h"
QVector<double> m36112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
