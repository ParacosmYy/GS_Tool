#include "g18726/m18726.h"
QVector<double> m18726::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
