#include "c15522/m15522.h"
QVector<double> m15522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
