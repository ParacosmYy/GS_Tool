#include "g9286/m9286.h"
QVector<double> m9286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
