#include "c35522/m35522.h"
QVector<double> m35522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
