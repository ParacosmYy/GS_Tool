#include "k35290/m35290.h"
QVector<double> m35290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
