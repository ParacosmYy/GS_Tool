#include "n9293/m9293.h"
QVector<double> m9293::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
