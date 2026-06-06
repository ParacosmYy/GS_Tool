#include "k17750/m17750.h"
QVector<double> m17750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
