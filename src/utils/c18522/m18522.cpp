#include "c18522/m18522.h"
QVector<double> m18522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
