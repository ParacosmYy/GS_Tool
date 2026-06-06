#include "k32290/m32290.h"
QVector<double> m32290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
