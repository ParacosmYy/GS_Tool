#include "t35979/m35979.h"
QVector<double> m35979::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
