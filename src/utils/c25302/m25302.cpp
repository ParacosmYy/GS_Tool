#include "c25302/m25302.h"
QVector<double> m25302::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
