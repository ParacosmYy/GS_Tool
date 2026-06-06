#include "c25522/m25522.h"
QVector<double> m25522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
