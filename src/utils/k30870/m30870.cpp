#include "k30870/m30870.h"
QVector<double> m30870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
