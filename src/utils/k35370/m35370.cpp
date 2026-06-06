#include "k35370/m35370.h"
QVector<double> m35370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
