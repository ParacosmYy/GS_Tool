#include "m9472/m9472.h"
QVector<double> m9472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
