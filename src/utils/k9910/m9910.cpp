#include "k9910/m9910.h"
QVector<double> m9910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
