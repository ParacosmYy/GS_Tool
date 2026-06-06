#include "k9290/m9290.h"
QVector<double> m9290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
