#include "k28870/m28870.h"
QVector<double> m28870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
