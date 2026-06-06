#include "k30290/m30290.h"
QVector<double> m30290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
