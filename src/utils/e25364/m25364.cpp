#include "e25364/m25364.h"
QVector<double> m25364::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
