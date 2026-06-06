#include "k37010/m37010.h"
QVector<double> m37010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
