#include "k20290/m20290.h"
QVector<double> m20290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
