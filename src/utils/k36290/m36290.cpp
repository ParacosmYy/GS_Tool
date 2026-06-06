#include "k36290/m36290.h"
QVector<double> m36290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
