#include "k18290/m18290.h"
QVector<double> m18290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
