#include "k15290/m15290.h"
QVector<double> m15290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
