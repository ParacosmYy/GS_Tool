#include "k10290/m10290.h"
QVector<double> m10290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
